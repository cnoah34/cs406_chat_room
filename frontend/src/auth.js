import { ref } from 'vue'

export const isAuthenticated = ref(!!localStorage.getItem('token'))

export function login(token, router) {
    localStorage.setItem('token', token)
    isAuthenticated.value = true
    router.push('/home')
}

export function logout(router) {
    localStorage.removeItem('token')
    isAuthenticated.value = false
    router.push('/')
}
