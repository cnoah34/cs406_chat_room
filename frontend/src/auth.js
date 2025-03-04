import { ref } from 'vue'
import axios from 'axios'
import { useUserStore } from '@/store/user'
import { useApiStore} from '@/store/api'


async function fetchUserData() {
    try {
        const apiStore = useApiStore()
        const response = await axios.get(`${apiStore.rest_url}/users/metadata`, {
            headers: {
                Authorization: `Bearer ${localStorage.getItem('token')}`
            }
        }) 

        const userStore = useUserStore()
        userStore.setUser(response.data)
        console.log(response.data)
    }
    catch (error) {
        console.error("Failed to fetch user meta data: ", error)
    }
}

export const isAuthenticated = ref(!!localStorage.getItem('token'))

export async function login(token, router) {
    localStorage.setItem('token', token)
    isAuthenticated.value = true
    await fetchUserData()
    router.push('/home')
}

export function logout(router) {
    localStorage.removeItem('token')
    isAuthenticated.value = false
    router.push('/')
}
