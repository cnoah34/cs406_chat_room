<template>
    <div class="rooms">
        <h1 style="color: white;">Your rooms</h1>
        <ul>
            <li v-for="room in roomsStore.rooms" :key="room.room_id" @click="selectRoom(room)">
                {{ room.name }}
            </li>
        </ul>
    </div>
</template>


<script setup>
    import { ref, onMounted } from 'vue'
    import { useUserStore } from '@/store/user'
    import { useRoomsStore } from '@/store/rooms'
    import { useApiStore } from '@/store/api'
    import axios from 'axios'

    const userStore = useUserStore()
    const roomsStore = useRoomsStore()
    const apiStore = useApiStore()

    const getRoom = async (room_id) => {
        try {
            const response = await axios.get(`${apiStore.rest_url}/rooms/${room_id}`, {
                headers: {
                    Authorization: `Bearer ${localStorage.getItem('token')}`
                }
            })

            if (response.status == 200 && response.data) {
                const room = response.data
                room.room_id = room_id  // Query does not include room_id
                roomsStore.addRoom(room)
            }
        }
        catch (error) {
            console.error('Failed to fetch room: ', error)
        }
    }

    const selectRoom = (room) => {
        roomsStore.setCurrentRoom(room)
    }

    onMounted(() => {
        userStore.room_ids.forEach(room_id => {
            // TODO: Add check to remove rooms user has been removed from
            if (!roomsStore.rooms.some(room => room.room_id === room_id)) {
                getRoom(room_id)
            }
        })
    })
</script>


<style scoped>

.rooms {
    display: flex;
    flex-direction: column;
    height: 80vh;
    padding-left: 20px;
    background-color: var(--foreground);
}

h1 {
    margin: 0;
    padding-bottom: 10px;
}

ul {
    list-style-type: none;
    margin: 0;
    padding: 0;
    color: var(--vue-green);
    cursor: pointer;
    text-decoration: underline;
    font-size: 14pt;
    overflow-y: auto;
}

ul:hover {
    text-decoration: none;
}

</style>
